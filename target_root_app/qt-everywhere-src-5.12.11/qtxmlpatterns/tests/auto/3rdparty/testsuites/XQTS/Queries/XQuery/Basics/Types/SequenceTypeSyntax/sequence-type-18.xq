(:*******************************************************:)
(:Test: sequence-type-18                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and integer. uses "integer?" on sequence of two strings.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

("abc","def") castable as xs:integer?