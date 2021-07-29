(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-15   :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-dateTime" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:dateTime("1978-12-12T07:07:07Z") - xs:yearMonthDuration("P17Y12M")) le xs:dateTime("1978-12-12T07:07:07Z")