(:*******************************************************:)
(: Test: default_namespace-018.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare default element namespace "http://www.example.com/filesystem";
declare function udf1 ()
{
  <empty>
   {10*10}
  </empty>
};
udf1 ()
