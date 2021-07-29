(:*******************************************************:)
(: Test: op-concatenate-mix-args-019.xq          :)
(: Written By: Ravindranath Chennnoju                    :)
(: Date: Thu May 12 23:38:31 2005                        :)
(: Purpose: Use an external variable with op:concatenate :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context//book/price, (), (1)
