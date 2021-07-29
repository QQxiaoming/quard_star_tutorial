(:*******************************************************:)
(:Test: op-subtract-yearMonthDuration-from-dateTime-7    :)
(:Written By: Carmelo Montanez                           :)
(:date: July 5, 2005                                     :)
(:Purpose: Evaluates The "subtract-yearMonthDuration-from-dateTime" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dateTime("1989-07-05T14:34:36Z") - xs:yearMonthDuration("P08Y04M"))