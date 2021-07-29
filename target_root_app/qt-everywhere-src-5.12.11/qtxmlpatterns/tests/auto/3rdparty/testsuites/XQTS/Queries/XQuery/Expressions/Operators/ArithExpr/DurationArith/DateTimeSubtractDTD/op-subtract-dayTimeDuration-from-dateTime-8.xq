(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-dateTime-8      :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The "subtract-dayTimeDuration-from-dateTime" operator that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:dateTime("0001-01-01T11:11:11Z") - xs:dayTimeDuration("-P11DT02H02M"))