(:*******************************************************:)
(:Test: op-numeric-add-derived-4                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates The "op:numeric-add" operator       :)
(: with the operators set as follows                     :)
(:$arg1 = xs:short                                       :)
(:$arg2 = xs:long                                       :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:short(10) + xs:long(145)