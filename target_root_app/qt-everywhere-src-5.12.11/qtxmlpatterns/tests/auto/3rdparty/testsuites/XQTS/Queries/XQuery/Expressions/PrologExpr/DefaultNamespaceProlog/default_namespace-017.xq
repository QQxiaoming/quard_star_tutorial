(:*******************************************************:)
(: Test: default_namespace-017.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare default element namespace "http://www.example.com/filesystem";
declare variable $v as xs:integer := 100;
declare function udf1 ($CUSTNO as xs:integer)
{
  <empty>
  {$CUSTNO*$v}
  </empty>
};
udf1(10)
