(:*******************************************************:)
(: Test: fn-union-node-args-005.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: processing-instruction node & node      :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context/processing-instruction() union $input-context/bib/book[2]/title
