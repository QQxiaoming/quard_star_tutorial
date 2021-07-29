(:*******************************************************:)
(:Test: op-add-yearMonthDurations-14                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-yearMonthDurations" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P21Y12M") + xs:yearMonthDuration("P08Y05M")) ne xs:yearMonthDuration("P08Y05M")