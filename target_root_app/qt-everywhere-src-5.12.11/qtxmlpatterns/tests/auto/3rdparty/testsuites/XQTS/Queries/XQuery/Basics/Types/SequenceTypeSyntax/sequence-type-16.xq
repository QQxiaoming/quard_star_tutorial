(:*******************************************************:)
(:Test: sequence-type-16                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and string. uses "string?" on sequence of two strings.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

("abc","def") castable as xs:string?