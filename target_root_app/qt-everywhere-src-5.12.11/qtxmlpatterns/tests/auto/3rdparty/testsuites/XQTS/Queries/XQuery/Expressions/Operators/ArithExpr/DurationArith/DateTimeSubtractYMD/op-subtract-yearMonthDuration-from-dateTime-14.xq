(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-14   :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-dateTime" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:dateTime("1979-12-12T09:09:09Z") - xs:yearMonthDuration("P08Y08M")) ne xs:dateTime("1979-12-12T09:09:09Z")