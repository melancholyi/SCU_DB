SELECT ProductName, CompanyName, ContactName
FROM(
	SELECT ProductName, min(OrderDate), CompanyName, ContactName
	FROM 
		(SELECT ID , ProductName
		FROM Product
		WHERE Product.Discontinued = 1) AS discon
		INNER JOIN OrderDetail on ProductId =  discon.Id 
		INNER JOIN 'Order' on 'Order'.Id = OrderDetail.OrderId
		INNER JOIN Customer on CustomerId = Customer.Id
		GROUP BY ProductName)
ORDER BY ProductName ASC
;
		
