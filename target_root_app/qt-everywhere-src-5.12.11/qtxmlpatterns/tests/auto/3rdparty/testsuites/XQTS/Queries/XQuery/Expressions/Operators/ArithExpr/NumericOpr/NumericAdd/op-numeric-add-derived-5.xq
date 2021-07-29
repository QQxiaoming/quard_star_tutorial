(:*******************************************************:)
(:Test: op-numeric-add-derived-5                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates The "op:numeric-add" operator       :)
(: with the operators set as follows                     :)
(:$arg1 = xs:positiveInteger                             :)
(:$arg2 = xs:negativeInteger                             :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:positiveInteger(10) + xs:negativeInteger(-5)