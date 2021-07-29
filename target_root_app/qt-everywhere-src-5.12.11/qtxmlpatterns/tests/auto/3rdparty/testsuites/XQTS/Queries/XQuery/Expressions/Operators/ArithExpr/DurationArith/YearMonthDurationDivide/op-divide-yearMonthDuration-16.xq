(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-16                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P13Y09M") div 2.0) ge xs:yearMonthDuration("P18Y02M")