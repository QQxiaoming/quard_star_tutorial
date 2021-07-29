(:*******************************************************:)
(:Test: op-numeric-add-derived-1                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 10, 2006                                    :)
(:Purpose: Evaluates The "op:numeric-add" operator       :)
(: with the operators set as follows                     :)
(:$arg1 = xs:long                                        :)
(:$arg2 = xs:unsignedLong                                :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

xs:long(10) + xs:unsignedLong(35)