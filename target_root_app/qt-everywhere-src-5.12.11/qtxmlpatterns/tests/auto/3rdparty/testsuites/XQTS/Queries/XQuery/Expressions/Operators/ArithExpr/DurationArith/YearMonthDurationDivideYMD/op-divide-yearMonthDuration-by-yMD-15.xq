(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-15            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") div xs:yearMonthDuration("P17Y02M")) le xs:decimal(2.0)