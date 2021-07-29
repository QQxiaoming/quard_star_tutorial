(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-13                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:together with the numeric-equal- operator "eq".        :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P23Y11M") div 2.0) eq xs:yearMonthDuration("P23Y11M")