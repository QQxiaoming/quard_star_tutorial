(:*******************************************************:)
(:Test: op-add-yearMonthDuration-to-date-8               :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "add-yearMonthDuration-to-date" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:date("0001-01-01Z") + xs:yearMonthDuration("-P20Y07M"))