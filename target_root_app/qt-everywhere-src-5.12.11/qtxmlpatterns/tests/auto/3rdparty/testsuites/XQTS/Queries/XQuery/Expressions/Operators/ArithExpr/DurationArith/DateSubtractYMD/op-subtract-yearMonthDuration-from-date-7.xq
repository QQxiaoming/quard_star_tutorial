(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-date-7        :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-date" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:date("1989-07-05Z") - xs:yearMonthDuration("P08Y04M"))