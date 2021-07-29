(:*******************************************************:)
(:Test: CastAs647                                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluates casting a string into a decimal.    :)
(:may process casting or raise error. This test may have different :)
(: outcomes.                                             :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)


xs:string(2.123456789123456789) cast as xs:decimal