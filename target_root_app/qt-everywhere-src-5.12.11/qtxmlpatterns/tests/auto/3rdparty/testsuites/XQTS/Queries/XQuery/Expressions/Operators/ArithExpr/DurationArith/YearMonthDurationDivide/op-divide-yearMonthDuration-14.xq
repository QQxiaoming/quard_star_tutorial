(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-14                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P21Y12M") div 2.0) ne xs:yearMonthDuration("P08Y05M")