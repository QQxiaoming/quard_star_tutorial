(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-11                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:yearMonthDuration("P20Y11M") * 2.0) div (xs:yearMonthDuration("P20Y11M") * 2.0)