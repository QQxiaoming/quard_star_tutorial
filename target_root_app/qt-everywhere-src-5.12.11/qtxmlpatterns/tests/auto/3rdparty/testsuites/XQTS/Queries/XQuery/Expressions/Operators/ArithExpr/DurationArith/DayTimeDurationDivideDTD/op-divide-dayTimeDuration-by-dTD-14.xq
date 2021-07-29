(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-14              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P21DT08H12M") div xs:dayTimeDuration("P08DT08H05M")) ne xs:decimal(2.0)