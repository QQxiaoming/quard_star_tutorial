(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-13       :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:date("1980-05-05Z") - xs:yearMonthDuration("P23Y11M")) eq xs:date("1980-05-05Z")