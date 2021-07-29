(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-11                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:together with a multiple "div" expressions.            :)
(:*******************************************************:)

(xs:yearMonthDuration("P20Y11M") div 2.0) div (xs:yearMonthDuration("P20Y11M") div 2.0)