(:*******************************************************:)
(: Test: fn-union-node-args-002.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: incorrect nodes                         :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context/bib/book/title | $input-context/bib/book
