(:*******************************************************:)
(: Test: default_namespace-014.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare function odd($x as xs:integer) as xs:boolean
{if ($x = 0) then fn:false() else even($x - 1)};
declare function even($x as xs:integer) as xs:boolean
{if ($x = 0) then fn:true() else odd($x - 1)};
even(4)
