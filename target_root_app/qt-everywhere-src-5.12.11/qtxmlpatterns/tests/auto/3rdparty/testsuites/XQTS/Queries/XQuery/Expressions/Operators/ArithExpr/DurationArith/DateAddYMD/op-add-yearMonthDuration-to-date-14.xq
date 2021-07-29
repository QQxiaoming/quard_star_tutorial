(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-14              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:date("1979-12-12Z") + xs:yearMonthDuration("P08Y08M")) ne xs:date("1979-12-12Z")