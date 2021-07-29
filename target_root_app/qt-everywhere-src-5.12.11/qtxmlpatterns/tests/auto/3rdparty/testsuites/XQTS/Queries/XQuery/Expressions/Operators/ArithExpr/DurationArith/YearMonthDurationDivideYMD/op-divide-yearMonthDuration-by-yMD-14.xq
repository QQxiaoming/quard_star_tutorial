(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-14            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P21Y12M") div xs:yearMonthDuration("P08Y05M")) ne xs:decimal(2.0)