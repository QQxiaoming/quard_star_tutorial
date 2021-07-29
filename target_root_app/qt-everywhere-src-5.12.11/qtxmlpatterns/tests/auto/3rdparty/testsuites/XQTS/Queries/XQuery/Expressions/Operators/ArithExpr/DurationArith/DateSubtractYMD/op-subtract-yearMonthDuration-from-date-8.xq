(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-8        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:date("0001-01-01Z") - xs:yearMonthDuration("-P20Y07M"))