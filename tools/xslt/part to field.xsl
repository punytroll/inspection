<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>
	
	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="/getter/part[value/append-value]">
		<field>
			<xsl:attribute name="name">
				<xsl:value-of select="value/append-value/name/text()"/>
			</xsl:attribute>
			<xsl:apply-templates select="node()[not(self::value)]"/>
		</field>
	</xsl:template>
	
	<xsl:template match="/getter/part[value/append-sub-values]">
		<fields>
			<xsl:apply-templates select="node()[not(self::value)]"/>
		</fields>
	</xsl:template>
	
	<xsl:template match="/getter/part[value/set]">
		<forward>
			<xsl:apply-templates select="node()[not(self::value)]"/>
		</forward>
	</xsl:template>
</xsl:stylesheet>
