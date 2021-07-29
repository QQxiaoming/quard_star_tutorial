(:*******************************************************:)
(: Test: default_namespace-002.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://www..oracle.com/xquery/test";
declare function foo($n as xs:integer)
{
   <tr> {$n} </tr>
};
foo(4)
