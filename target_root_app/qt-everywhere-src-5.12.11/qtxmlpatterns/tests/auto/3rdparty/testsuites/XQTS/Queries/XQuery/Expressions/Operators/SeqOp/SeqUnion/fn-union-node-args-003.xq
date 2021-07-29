(:*******************************************************:)
(: Test: fn-union-node-args-003.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: node                                    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context/bib/book[3]/title | root(fn:exactly-one($input-context/bib/book[3]/title))
