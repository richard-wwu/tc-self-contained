/*
 * Copyright 2011,2015 Sven Verdoolaege. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SVEN VERDOOLAEGE ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SVEN VERDOOLAEGE OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as
 * representing official policies, either expressed or implied, of
 * Sven Verdoolaege.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>

#include <clang/AST/Attr.h>
#include <clang/Basic/SourceManager.h>

#include "isl_config.h"
#include "extract_interface.h"
#include "generator.h"

/* Compare the prefix of "s" to "prefix" up to the length of "prefix".
 */
static int prefixcmp(const char *s, const char *prefix)
{
	return strncmp(s, prefix, strlen(prefix));
}

const char *isl_class::set_callback_prefix = "set_";

/* Should "method" be considered to be a static method?
 * That is, is the first argument something other than
 * an instance of the class?
 */
bool generator::is_static(const isl_class &clazz, FunctionDecl *method)
{
	ParmVarDecl *param;
	QualType type;

	if (method->getNumParams() < 1)
		return true;

	param = method->getParamDecl(0);
	type = param->getOriginalType();
	if (!is_isl_type(type))
		return true;
	return extract_type(type) != clazz.name;
}

/* Does "fd" modify an object of "clazz"?
 * That is, is it an object method that takes the object and
 * returns (gives) an object of the same type?
 */
bool generator::is_mutator(const isl_class &clazz, FunctionDecl *fd)
{
	ParmVarDecl *param;
	QualType type, return_type;

	if (fd->getNumParams() < 1)
		return false;
	if (is_static(clazz, fd))
		return false;

	if (!gives(fd))
		return false;
	param = fd->getParamDecl(0);
	if (!takes(param))
		return false;
	type = param->getOriginalType();
	return_type = fd->getReturnType();
	return return_type == type;
}

/* Find the FunctionDecl with name "name",
 * returning NULL if there is no such FunctionDecl.
 * If "required" is set, then error out if no FunctionDecl can be found.
 */
FunctionDecl *generator::find_by_name(const string &name, bool required)
{
	map<string, FunctionDecl *>::iterator i;

	i = functions_by_name.find(name);
	if (i != functions_by_name.end())
		return i->second;
	if (required)
		die("No " + name + " function found");
	return NULL;
}

/* Add a subclass derived from "decl" called "sub_name" to the set of classes,
 * keeping track of the _to_str, _copy and _free functions, if any, separately.
 * "sub_name" is either the name of the class itself or
 * the name of a type based subclass.
 * If the class is a proper subclass, then "super_name" is the name
 * of its immediate superclass.
 */
void generator::add_subclass(RecordDecl *decl, const string &super_name,
	const string &sub_name)
{
	string name = decl->getName();

	classes[sub_name].name = name;
	classes[sub_name].superclass_name = super_name;
	classes[sub_name].subclass_name = sub_name;
	classes[sub_name].type = decl;
	classes[sub_name].fn_to_str = find_by_name(name + "_to_str", false);
	classes[sub_name].fn_copy = find_by_name(name + "_copy", true);
	classes[sub_name].fn_is_equal = find_by_name(name + "_is_equal", false);
	classes[sub_name].fn_free = find_by_name(name + "_free", true);
}

/* Add a class derived from "decl" to the set of classes,
 * keeping track of the _to_str, _copy and _free functions, if any, separately.
 */
void generator::add_class(RecordDecl *decl)
{
	return add_subclass(decl, "", decl->getName());
}

/* Given a function "fn_type" that returns the subclass type
 * of a C object, create subclasses for each of the (non-negative)
 * return values.
 *
 * The function "fn_type" is also stored in the superclass,
 * along with all pairs of type values and subclass names.
 */
void generator::add_type_subclasses(FunctionDecl *fn_type)
{
	QualType return_type = fn_type->getReturnType();
	const EnumType *enum_type = return_type->getAs<EnumType>();
	EnumDecl *decl = enum_type->getDecl();
	isl_class *c = method2class(fn_type);
	DeclContext::decl_iterator i;

	c->fn_type = fn_type;
	for (i = decl->decls_begin(); i != decl->decls_end(); ++i) {
		EnumConstantDecl *ecd = dyn_cast<EnumConstantDecl>(*i);
		int val = (int) ecd->getInitVal().getExtValue();
		string name = ecd->getNameAsString();

		if (val < 0)
			continue;
		c->type_subclasses[val] = name;
		add_subclass(c->type, c->subclass_name, name);
	}
}

/* Add information about the enum values in "decl", set by "fd",
 * to c->set_enums. "prefix" is the prefix of the generated method names.
 * In particular, it has the name of the enum type removed.
 *
 * In particular, for each non-negative enum value, keep track of
 * the value, the name and the corresponding method name.
 */
static void add_set_enum(isl_class *c, const string &prefix, EnumDecl *decl,
	FunctionDecl *fd)
{
	DeclContext::decl_iterator i;

	for (i = decl->decls_begin(); i != decl->decls_end(); ++i) {
		EnumConstantDecl *ecd = dyn_cast<EnumConstantDecl>(*i);
		int val = (int) ecd->getInitVal().getSExtValue();
		string name = ecd->getNameAsString();
		string method_name;

		if (val < 0)
			continue;
		method_name = prefix + name.substr(4);
		c->set_enums[fd].emplace_back(set_enum(val, name, method_name));
	}
}

/* Check if "fd" sets an enum value and, if so, add information
 * about the enum values to c->set_enums.
 *
 * A function is considered to set an enum value if:
 * - the function returns an object of the same type
 * - the last argument is of type enum
 * - the name of the function ends with the name of the enum
 */
static bool handled_sets_enum(isl_class *c, FunctionDecl *fd)
{
	unsigned n;
	ParmVarDecl *param;
	const EnumType *enum_type;
	EnumDecl *decl;
	string enum_name;
	string fd_name;
	string prefix;
	size_t pos;

	if (!generator::is_mutator(*c, fd))
		return false;
	n = fd->getNumParams();
	if (n < 2)
		return false;
	param = fd->getParamDecl(n - 1);
	enum_type = param->getType()->getAs<EnumType>();
	if (!enum_type)
		return false;
	decl = enum_type->getDecl();
	enum_name = decl->getName();
	enum_name = enum_name.substr(4);
	fd_name = c->method_name(fd);
	pos = fd_name.find(enum_name);
	if (pos == std::string::npos)
		return false;
	prefix = fd_name.substr(0, pos);

	add_set_enum(c, prefix, decl, fd);

	return true;
}

/* Return the callback argument of a function setting
 * a persistent callback.
 * This callback is in the second argument (position 1).
 */
ParmVarDecl *generator::persistent_callback_arg(FunctionDecl *fd)
{
	return fd->getParamDecl(1);
}

/* Does the given function set a persistent callback?
 * The following heuristics are used to determine this property:
 * - the function returns an object of the same type
 * - its name starts with "set_"
 * - it has exactly three arguments
 * - the second (position 1) of which is a callback
 */
static bool sets_persistent_callback(isl_class *c, FunctionDecl *fd)
{
	ParmVarDecl *param;

	if (!generator::is_mutator(*c, fd))
		return false;
	if (fd->getNumParams() != 3)
		return false;
	param = generator::persistent_callback_arg(fd);
	if (!generator::is_callback(param->getType()))
		return false;
	return prefixcmp(c->method_name(fd).c_str(),
			 c->set_callback_prefix) == 0;
}

/* Collect all functions that belong to a certain type, separating
 * constructors from methods that set persistent callback and
 * from regular methods, while keeping track of the _to_str,
 * _copy and _free functions, if any, separately.  If there are any overloaded
 * functions, then they are grouped based on their name after removing the
 * argument type suffix.
 * Check for functions that describe subclasses before considering
 * any other functions in order to be able to detect those other
 * functions as belonging to the subclasses.
 */
generator::generator(SourceManager &SM, set<RecordDecl *> &exported_types,
	set<FunctionDecl *> exported_functions, set<FunctionDecl *> functions) :
	SM(SM)
{
	map<string, isl_class>::iterator ci;

	set<FunctionDecl *>::iterator in;
	for (in = functions.begin(); in != functions.end(); ++in) {
		FunctionDecl *decl = *in;
		functions_by_name[decl->getName()] = decl;
	}

	set<RecordDecl *>::iterator it;
	for (it = exported_types.begin(); it != exported_types.end(); ++it)
		add_class(*it);

	for (in = exported_functions.begin(); in != exported_functions.end();
	     ++in) {
		if (!is_subclass(*in))
			continue;
		add_type_subclasses(*in);
	}

	for (in = exported_functions.begin(); in != exported_functions.end();
	     ++in) {
		isl_class *c;
		FunctionDecl *method = *in;

		if (is_subclass(method))
			continue;

		c = method2class(method);
		if (!c)
			continue;
		if (is_constructor(method)) {
			c->constructors.insert(method);
		} else if (handled_sets_enum(c, method)) {
		} else if (sets_persistent_callback(c, method)) {
			c->persistent_callbacks.insert(method);
		} else {
			string fullname = c->name_without_type_suffix(method);
			c->methods[fullname].insert(method);
		}
	}
}

/* Print error message "msg" and abort.
 */
void generator::die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

/* Print error message "msg" and abort.
 */
void generator::die(string msg)
{
	die(msg.c_str());
}

/* Return a sequence of the types of which the given type declaration is
 * marked as being a subtype.
 * The order of the types is the opposite of the order in which they
 * appear in the source.  In particular, the first annotation
 * is the one that is closest to the annotated type and the corresponding
 * type is then also the first that will appear in the sequence of types.
 */
std::vector<string> generator::find_superclasses(Decl *decl)
{
	vector<string> super;

	if (!decl->hasAttrs())
		return super;

	string sub = "isl_subclass";
	size_t len = sub.length();
	AttrVec attrs = decl->getAttrs();
	for (AttrVec::const_iterator i = attrs.begin(); i != attrs.end(); ++i) {
		const AnnotateAttr *ann = dyn_cast<AnnotateAttr>(*i);
		if (!ann)
			continue;
		string s = ann->getAnnotation().str();
		if (s.substr(0, len) == sub) {
			s = s.substr(len + 1, s.length() - len  - 2);
			super.push_back(s);
		}
	}

	return super;
}

/* Is "decl" marked as describing subclasses?
 */
bool generator::is_subclass(FunctionDecl *decl)
{
	return find_superclasses(decl).size() > 0;
}

/* Is decl marked as being part of an overloaded method?
 */
bool generator::is_overload(Decl *decl)
{
	return has_annotation(decl, "isl_overload");
}

/* Is decl marked as a constructor?
 */
bool generator::is_constructor(Decl *decl)
{
	return has_annotation(decl, "isl_constructor");
}

/* Is decl marked as consuming a reference?
 */
bool generator::takes(Decl *decl)
{
	return has_annotation(decl, "isl_take");
}

/* Is decl marked as preserving a reference?
 */
bool generator::keeps(Decl *decl)
{
	return has_annotation(decl, "isl_keep");
}

/* Is decl marked as returning a reference that is required to be freed.
 */
bool generator::gives(Decl *decl)
{
	return has_annotation(decl, "isl_give");
}

/* Return the class that has a name that best matches the initial part
 * of the name of function "fd" or NULL if no such class could be found.
 */
isl_class *generator::method2class(FunctionDecl *fd)
{
	string best;
	map<string, isl_class>::iterator ci;
	string name = fd->getNameAsString();

	for (ci = classes.begin(); ci != classes.end(); ++ci) {
		size_t len = ci->first.length();
		if (len > best.length() && name.substr(0, len) == ci->first)
			best = ci->first;
	}

	if (classes.find(best) == classes.end()) {
		cerr << "Unable to find class of " << name << endl;
		return NULL;
	}

	return &classes[best];
}

/* Is "type" the type "isl_ctx *"?
 */
bool generator::is_isl_ctx(QualType type)
{
	if (!type->isPointerType())
		return 0;
	type = type->getPointeeType();
	if (type.getAsString() != "isl_ctx")
		return false;

	return true;
}

/* Is the first argument of "fd" of type "isl_ctx *"?
 */
bool generator::first_arg_is_isl_ctx(FunctionDecl *fd)
{
	ParmVarDecl *param;

	if (fd->getNumParams() < 1)
		return false;

	param = fd->getParamDecl(0);
	return is_isl_ctx(param->getOriginalType());
}

namespace {

struct ClangAPI {
	/* Return the first location in the range returned by
	 * clang::SourceManager::getImmediateExpansionRange.
	 * Older versions of clang return a pair of SourceLocation objects.
	 * More recent versions return a CharSourceRange.
	 */
	static SourceLocation range_begin(
			const std::pair<SourceLocation,SourceLocation> &p) {
		return p.first;
	}
	static SourceLocation range_begin(const CharSourceRange &range) {
		return range.getBegin();
	}
};

}

/* Does the callback argument "param" take its argument at position "pos"?
 *
 * The memory management annotations of arguments to function pointers
 * are not recorded by clang, so the information cannot be extracted
 * from the type of "param".
 * Instead, go to the location in the source where the callback argument
 * is declared, look for the right argument of the callback itself and
 * then check if it has an "__isl_take" memory management annotation.
 *
 * If the return value of the function has a memory management annotation,
 * then the spelling of "param" will point to the spelling
 * of this memory management annotation.  Since the macro is defined
 * on the command line (in main), this location does not have a file entry.
 * In this case, move up one level in the macro expansion to the location
 * where the memory management annotation is used.
 */
bool generator::callback_takes_argument(ParmVarDecl *param,
	int pos)
{
	SourceLocation loc;
	const char *s, *end, *next;
	bool takes, keeps;

	loc = param->getSourceRange().getBegin();
	if (!SM.getFileEntryForID(SM.getFileID(SM.getSpellingLoc(loc))))
		loc = ClangAPI::range_begin(SM.getImmediateExpansionRange(loc));
	s = SM.getCharacterData(loc);
	if (!s)
		die("No character data");
	s = strchr(s, '(');
	if (!s)
		die("Cannot find function pointer");
	s = strchr(s + 1, '(');
	if (!s)
		die("Cannot find function pointer arguments");
	end = strchr(s + 1, ')');
	if (!end)
		die("Cannot find end of function pointer arguments");
	while (pos-- > 0) {
		s = strchr(s + 1, ',');
		if (!s || s > end)
			die("Cannot find function pointer argument");
	}
	next = strchr(s + 1, ',');
	if (next && next < end)
		end = next;
	s = strchr(s + 1, '_');
	if (!s || s > end)
		die("Cannot find function pointer argument annotation");
	takes = prefixcmp(s, "__isl_take") == 0;
	keeps = prefixcmp(s, "__isl_keep") == 0;
	if (!takes && !keeps)
		die("Cannot find function pointer argument annotation");

	return takes;
}

/* Is "type" that of a pointer to an isl_* structure?
 */
bool generator::is_isl_type(QualType type)
{
	if (type->isPointerType()) {
		string s;

		type = type->getPointeeType();
		if (type->isFunctionType())
			return false;
		s = type.getAsString();
		return s.substr(0, 4) == "isl_";
	}

	return false;
}

/* Is "type" the type isl_bool?
 */
bool generator::is_isl_bool(QualType type)
{
	string s;

	if (type->isPointerType())
		return false;

	s = type.getAsString();
	return s == "isl_bool";
}

/* Is "type" the type isl_stat?
 */
bool generator::is_isl_stat(QualType type)
{
	string s;

	if (type->isPointerType())
		return false;

	s = type.getAsString();
	return s == "isl_stat";
}

/* Is "type" that of a pointer to a function?
 */
bool generator::is_callback(QualType type)
{
	if (!type->isPointerType())
		return false;
	type = type->getPointeeType();
	return type->isFunctionType();
}

/* Is "type" that of "char *" of "const char *"?
 */
bool generator::is_string(QualType type)
{
	if (type->isPointerType()) {
		string s = type->getPointeeType().getAsString();
		return s == "const char" || s == "char";
	}

	return false;
}

/* Is "type" that of "long"?
 */
bool generator::is_long(QualType type)
{
	const BuiltinType *builtin = type->getAs<BuiltinType>();
	return builtin && builtin->getKind() == BuiltinType::Long;
}

/* Return the name of the type that "type" points to.
 * The input "type" is assumed to be a pointer type.
 */
string generator::extract_type(QualType type)
{
	if (type->isPointerType())
		return type->getPointeeType().getAsString();
	die("Cannot extract type from non-pointer type");
}

/* Given the type of a function pointer, return the corresponding
 * function prototype.
 */
const FunctionProtoType *generator::extract_prototype(QualType type)
{
	return type->getPointeeType()->getAs<FunctionProtoType>();
}

/* If "method" is overloaded, then return its name with the suffix
 * corresponding to the type of the final argument removed.
 * Otherwise, simply return the name of the function.
 */
string isl_class::name_without_type_suffix(FunctionDecl *method)
{
	int num_params;
	ParmVarDecl *param;
	string name, type;
	size_t name_len, type_len;

	name = method->getName();
	if (!generator::is_overload(method))
		return name;

	num_params = method->getNumParams();
	param = method->getParamDecl(num_params - 1);
	type = generator::extract_type(param->getOriginalType());
	type = type.substr(4);
	name_len = name.length();
	type_len = type.length();

	if (name_len > type_len && name.substr(name_len - type_len) == type)
		name = name.substr(0, name_len - type_len - 1);

	return name;
}
