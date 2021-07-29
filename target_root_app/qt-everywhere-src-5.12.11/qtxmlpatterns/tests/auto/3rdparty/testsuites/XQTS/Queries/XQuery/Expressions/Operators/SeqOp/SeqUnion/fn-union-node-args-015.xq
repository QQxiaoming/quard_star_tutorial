(:*******************************************************:)
(: Test: fn-union-node-args-015.xq          :)
(: Written By: Ravindranath Chennoju                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: node and node                     :)
(:*******************************************************:)

(: insert-start :)
declare namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

($input-context/atomic:root/atomic:integer) union ($input-context/atomic:root/atomic:integer)
