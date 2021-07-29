(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-13            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with the numeric-equal- operator "eq".        :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P23Y11M") div xs:yearMonthDuration("P23Y11M")) eq xs:decimal(2.0)