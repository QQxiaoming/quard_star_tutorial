(:********************************************************************:)
(: Test: function-declaration-003.xq                                  :)
(: Written By: Pulkita Tyagi                                          :)
(: Date: Thu Jun  2 00:24:56 2005                                     :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:price ($i as element()) as element()?
{
  $i/price
};
for $j in $input-context/bib/book
  return local:price($j)
