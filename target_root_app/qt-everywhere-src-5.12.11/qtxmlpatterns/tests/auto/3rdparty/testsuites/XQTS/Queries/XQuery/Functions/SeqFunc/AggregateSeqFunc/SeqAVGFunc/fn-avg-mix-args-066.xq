(:*******************************************************:)
(: Test: fn-avg-mix-args-066.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0001                 :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:avg(($input-context/bib/book[1]))
