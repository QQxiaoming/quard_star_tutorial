(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-15                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") div 2.0) le xs:yearMonthDuration("P17Y02M")