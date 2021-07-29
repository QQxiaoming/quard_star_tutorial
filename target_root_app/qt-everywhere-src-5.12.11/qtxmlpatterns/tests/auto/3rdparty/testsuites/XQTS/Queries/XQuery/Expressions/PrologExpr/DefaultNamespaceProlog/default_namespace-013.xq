(:*******************************************************:)
(: Test: default_namespace-013.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare function foo2($i as xs:string) as xs:string
{foo($i)};
declare function foo($i as xs:string) as xs:string
{$i};
foo2("abc")
