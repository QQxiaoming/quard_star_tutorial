xquery version "1.0";
(:*******************************************************:)
(: Test: version_declaration-005.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 04:11:16 2005                        :)
(: Purpose: Demonstrates version declaration,the version :)
(:          declaration occurs at the beginning of the   :)
(:          module and identifies the applicable XQuery  :)
(:          syntax and semantics for the module          :)
(:*******************************************************:)

declare boundary-space preserve;
declare default collation "xspanish";
declare base-uri "http://www.base-uri.com/";
declare construction strip;
declare ordering unordered;
declare default order empty greatest;
import schema default element namespace "http://comment";
declare namespace foo = "http://foo.org";
declare default function namespace "http://example.org/math-functions";
declare namespace bar = "http://bar.org";
declare variable $x as xs:integer := 7;
declare function local:sumOf1toN($i) as xs:integer
{
  if ($i = 1)
  then $i
  else $i + local:sumOf1toN($i - 1)
};
declare option foo:strip-comments "true";
local:sumOf1toN(5)
