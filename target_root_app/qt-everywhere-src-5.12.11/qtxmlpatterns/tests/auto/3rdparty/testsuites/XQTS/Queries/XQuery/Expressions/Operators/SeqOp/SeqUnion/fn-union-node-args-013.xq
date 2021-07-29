(:*******************************************************:)
(: Test: fn-union-node-args-013.xq          :)
(: Written By: Ravindranath Chennoju                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: node & empty sequence                     :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context//author | () 
