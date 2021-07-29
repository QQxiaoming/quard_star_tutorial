(:*******************************************************:)
(:Test: op-numeric-add-derived-3                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates The "op:numeric-add" operator       :)
(: with the operators set as follows                     :)
(:$arg1 = xs:nonNegativeInteger                          :)
(:$arg2 = xs:short                                       :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:nonNegativeInteger(10) + xs:short(15)