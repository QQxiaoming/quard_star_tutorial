(:*******************************************************:)
(: Test: op-concatenate-mix-args-020.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Thu May 12 23:38:31 2005                        :)
(: Purpose: Use two external variables with op:contenate :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context//book/price, $input-context//book/title
