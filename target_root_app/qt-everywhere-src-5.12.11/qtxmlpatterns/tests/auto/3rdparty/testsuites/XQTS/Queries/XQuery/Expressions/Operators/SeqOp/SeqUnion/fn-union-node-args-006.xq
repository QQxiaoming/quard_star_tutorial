(:*******************************************************:)
(: Test: fn-union-node-args-006.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: processing-instruction node & node      :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context/processing-instruction(PI1) union $input-context/bib/book[3]/title
