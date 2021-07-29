(:*******************************************************:)
(: Test: default_namespace-006.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare function mysum($i as xs:integer, $j as xs:integer)
{
   let $j := $i + $j
   return $j
};
declare function invoke_mysum($st as xs:integer)
{
  for $d in (1,2,3,4,5)
  let $st := mysum($d, $st)
   return $st
};
invoke_mysum(0)
