(:*******************************************************:)
(: Test: default_namespace-003.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://www..oracle.com/xquery/test";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function price ($i as element()) as element()?
{
  $i/price
};
for $j in $input-context/bib/book
  return price($j)
