(:*******************************************************:)
(:Test: sequence-type-14                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and decimal. uses "decimal?" on sequence of two decimals.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1.2,2.1) castable as xs:decimal?