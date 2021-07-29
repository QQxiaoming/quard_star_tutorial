(:*******************************************************:)
(: Test: fn-union-node-args-012.xq          :)
(: Written By: Ravindranath Chennoju                             :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg:  node & non existing node                     :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context//author union $input-context//nonexisting
