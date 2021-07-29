(:*******************************************************:)
(: Test: fn-union-node-args-017.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: node & node                     :)
(:*******************************************************:)

(: insert-start :)
declare namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

($input-context/atomic:root/atomic:integer) | ($input-context/atomic:root/atomic:string)

