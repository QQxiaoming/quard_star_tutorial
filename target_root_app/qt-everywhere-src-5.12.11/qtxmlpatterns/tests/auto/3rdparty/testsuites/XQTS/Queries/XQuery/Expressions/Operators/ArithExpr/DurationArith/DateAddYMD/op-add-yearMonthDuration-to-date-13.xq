(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-13              :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
(xs:date("1980-05-05Z") + xs:yearMonthDuration("P23Y11M")) eq xs:date("1980-05-05Z")