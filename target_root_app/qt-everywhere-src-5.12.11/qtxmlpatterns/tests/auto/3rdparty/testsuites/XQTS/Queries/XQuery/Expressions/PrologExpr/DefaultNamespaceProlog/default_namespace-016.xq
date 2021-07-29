(:*******************************************************:)
(: Test: default_namespace-016.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function title($a_book as element()) as element()*
{
for $i in $a_book return $i/title
};
$input-context/bib/book/(title(.))
