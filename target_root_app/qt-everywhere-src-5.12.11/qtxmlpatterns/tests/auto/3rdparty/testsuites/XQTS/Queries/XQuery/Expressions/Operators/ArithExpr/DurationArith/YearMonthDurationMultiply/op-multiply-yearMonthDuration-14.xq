(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-14                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P21Y12M") * 2.0) ne xs:yearMonthDuration("P08Y05M")