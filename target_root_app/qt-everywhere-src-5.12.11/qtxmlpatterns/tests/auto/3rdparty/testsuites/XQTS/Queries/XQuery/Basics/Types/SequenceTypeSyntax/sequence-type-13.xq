(:*******************************************************:)
(:Test: sequence-type-13                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and decimal. uses "decimal?" on sequence of one decimal.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1.2) castable as xs:decimal?