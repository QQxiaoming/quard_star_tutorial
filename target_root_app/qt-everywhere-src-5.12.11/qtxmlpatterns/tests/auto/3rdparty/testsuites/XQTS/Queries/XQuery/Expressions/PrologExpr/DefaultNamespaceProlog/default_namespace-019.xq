(:*******************************************************:)
(: Test: default_namespace-019.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare default element namespace "http://www.example.com/def";
declare namespace test="http://www.example.com/test";
declare namespace test2="http://www.example.com/test2";
declare function test:udf1()
{
  <empty>
    {10*10}
</empty>
};
declare function test2:udf1()
{
  <empty/>
};
<A>
{test:udf1()}
{test2:udf1()}
</A>
