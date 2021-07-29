(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-16   :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 5, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-dateTime" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:dateTime("1977-12-12T03:03:03Z") - xs:yearMonthDuration("P18Y02M")) ge  xs:dateTime("1977-12-12T03:03:03Z")