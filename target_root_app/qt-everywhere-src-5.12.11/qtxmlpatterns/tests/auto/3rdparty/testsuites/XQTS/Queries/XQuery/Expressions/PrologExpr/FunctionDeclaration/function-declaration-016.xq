(:*******************************************************:)
(: Test: function-declaration-016.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:title($a_book as element())
{
for $i in $a_book return $i/title
};
$input-context/bib/book/(local:title(.))
