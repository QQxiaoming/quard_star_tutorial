(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-15                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") * 2.0) le xs:yearMonthDuration("P17Y02M")