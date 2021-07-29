(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-16            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P13Y09M") div xs:yearMonthDuration("P18Y02M")) ge xs:decimal(2.0)