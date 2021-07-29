(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-13                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" function used  :)
(:together with the numeric-equal- operator "eq".        :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P23Y11M") * 2.0) eq xs:yearMonthDuration("P23Y11M")