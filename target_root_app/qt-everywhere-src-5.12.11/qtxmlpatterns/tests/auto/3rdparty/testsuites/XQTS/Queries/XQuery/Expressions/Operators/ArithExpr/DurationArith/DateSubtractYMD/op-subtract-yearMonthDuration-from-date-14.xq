(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-14       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:date("1979-12-12Z") - xs:yearMonthDuration("P08Y08M")) ne xs:date("1979-12-12Z")