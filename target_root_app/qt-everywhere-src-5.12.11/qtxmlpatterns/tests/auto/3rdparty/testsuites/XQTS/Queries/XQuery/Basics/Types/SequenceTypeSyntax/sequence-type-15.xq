(:*******************************************************:)
(:Test: sequence-type-15                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and string. uses "string?" on sequence of one string.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

("abc") castable as xs:string?